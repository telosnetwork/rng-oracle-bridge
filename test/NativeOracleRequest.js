const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");

describe("NativeOracleRequest Contract", function () {
    let bridge, owner, oracle, oracle2, user;
    beforeEach(async () => {
        [owner, oracle, oracle2, user] = await ethers.getSigners();
        let Bridge = await ethers.getContractFactory("NativeOracleRequest");
        bridge = await Bridge.deploy(ONE_TLOS);
    })
    describe(":: Settings", function () {
        it("Owner should be able to register an Oracle" , async function () {
            await expect(bridge.registerOracle(oracle.address, "mygreatoracle")).to.not.be.reverted;
        });
        it("No other address should be able to add an Oracle" , async function () {
            await expect(bridge.connect(user).addOracle(oracle2.address, "mygreatoracle2")).to.be.reverted;
        });
        it("No other address should be able to remove an Oracle" , async function () {
            await expect(bridge.connect(user).removeOracle(oracle.address)).to.be.reverted;
        });
        it("Owner should be able to remove an Oracle" , async function () {
            await expect(bridge.removeOracle(oracle.address)).to.not.be.reverted;
        });
        it("You should be able to query the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
    });
    describe(":: Getters", function () {
        it("You should be able to query the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
    });
    describe(":: Request", function () {
        it("Request should be created with correct parameters" , async function () {
        });
        it("Request should revert if fee is incorrect" , async function () {
        });
        it("Request should revert if oracle is not registered" , async function () {
        });
        it("Request should revert if callId already exists" , async function () {
        });
    });
    describe(":: Response", function () {
        it("Response should delete the Request" , async function () {
        });
    });
});
